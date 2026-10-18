USE [GunzDB]
GO

SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO

/* Trả về giá trị nhỏ nhất giữa 2 số */
CREATE FUNCTION [dbo].[fnGetMin]
(
    @n1 INT,
    @n2 INT
)
RETURNS INT
AS
BEGIN
    RETURN (CASE WHEN @n1 < @n2 THEN @n1 ELSE @n2 END)
END
GO
